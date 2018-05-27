/*++

Copyright (c) 2018 Trent Nelson <trent@trent.me>

Module Name:

    Chm_01.c

Abstract:

    This module implements the CHM perfect hash table algorithm.  As a time
    saving measure, this module contains *everything* pertaining to the CHM
    implementation, including more general structures like hypergraphs etc.

    The general structures and functions can be moved out into their own
    modules at a later date if we author other algorithms that wish to use them.

    N.B. This implementation attempts to mirror the chm.c implementation as
         best it can, including but not limited to the underlying algorithm
         approach and function names.  This will be used as the baseline for
         evaluating the performance of subsequent revisions.

--*/

#include "stdafx.h"

_Use_decl_annotations_
BOOLEAN
CreatePerfectHashTableImplChm01(
    PPERFECT_HASH_TABLE Table
    )
/*++

Routine Description:

    Attempts to create a perfect hash table using the CHM algorithm and a
    2-part random hypergraph.

Arguments:

    Table - Supplies a pointer to a partially-initialized PERFECT_HASH_TABLE
        structure.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    PRTL Rtl;
    USHORT Index;
    PULONG Keys;
    PGRAPH Graph;
    PBYTE Buffer;
    BOOLEAN Success;
    USHORT PageSize;
    USHORT PageShift;
    ULONG_PTR LastPage;
    ULONG_PTR ThisPage;
    PVOID BaseAddress = NULL;
    ULONG WaitResult;
    GRAPH_INFO Info;
    PBYTE Unusable;
    BOOLEAN CaughtException;
    PALLOCATOR Allocator;
    HANDLE ProcessHandle;
    USHORT NumberOfGraphs;
    USHORT NumberOfPagesPerGraph;
    USHORT NumberOfGuardPages;
    USHORT TotalNumberOfPages;
    USHORT NumberOfBitmaps;
    ULONGLONG NextSizeInBytes;
    ULONGLONG PrevSizeInBytes;
    ULONGLONG FirstSizeInBytes;
    ULONGLONG EdgesSizeInBytes;
    ULONGLONG AssignedSizeInBytes;
    ULONGLONG TotalBufferSizeInBytes;
    ULONGLONG UsableBufferSizeInBytesPerBuffer;
    ULONGLONG ExpectedTotalBufferSizeInBytes;
    ULONGLONG ExpectedUsableBufferSizeInBytesPerBuffer;
    ULONGLONG GraphSizeInBytesIncludingGuardPage;
    ULARGE_INTEGER AllocSize;
    ULARGE_INTEGER NumberOfEdges;
    ULARGE_INTEGER NumberOfVertices;
    ULARGE_INTEGER TotalNumberOfEdges;
    ULARGE_INTEGER DeletedEdgesBitmapBufferSizeInBytes;
    ULARGE_INTEGER VisitedVerticesBitmapBufferSizeInBytes;
    PPERFECT_HASH_TABLE_CONTEXT Context = Table->Context;
    HANDLE Events[4];
    USHORT NumberOfEvents = ARRAYSIZE(Events);


    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(Table)) {
        return FALSE;
    }

    //
    // Initialize aliases.
    //

    Rtl = Table->Rtl;
    Keys = (PULONG)Table->Keys->BaseAddress;;
    Allocator = Table->Allocator;
    Context = Table->Context;

    //
    // The number of edges in our graph is equal to the number of keys in the
    // input data set.
    //

    NumberOfEdges.QuadPart = Table->Keys->NumberOfElements.QuadPart;

    //
    // Sanity check we're under MAX_ULONG.
    //

    ASSERT(!NumberOfEdges.HighPart);

    //
    // The r-graph (r = 2) nature of this implementation results in various
    // arrays having twice the number of elements indicated by the edge count.
    // Capture this number now, as we need it in various size calculations.
    //

    TotalNumberOfEdges.QuadPart = NumberOfEdges.QuadPart;
    TotalNumberOfEdges.QuadPart <<= 1;

    //
    // Another overflow sanity check.
    //

    ASSERT(!TotalNumberOfEdges.HighPart);

    //
    // chm.c uses a size multiplier (c) of 2.09.  Let's avoid the need for
    // doubles and linking with a math library in order to get ceil(), and
    // just use 2.5, which we can calculate by adding the result of right
    // shifting the number of edges by 1 to the result of left shifting
    // said edge count by 1 (simulating multiplication by 0.5).
    //

    NumberOfVertices.QuadPart = NumberOfEdges.LowPart << 1;
    NumberOfVertices.QuadPart += NumberOfEdges.LowPart >> 1;

    //
    // Another sanity check we haven't exceeded MAX_ULONG.
    //

    ASSERT(!NumberOfVertices.HighPart);

    //
    // Calculate the size required for the DeletedEdges bitmap buffer.  One
    // bit is used per TotalNumberOfEdges.  Convert the bits into bytes by
    // shifting right 3 (dividing by 8) then align it up to a 16 byte boundary.
    //

    DeletedEdgesBitmapBufferSizeInBytes.QuadPart = (
        ALIGN_UP((TotalNumberOfEdges.QuadPart >> 3), 16)
    );

    ASSERT(!DeletedEdgesBitmapBufferSizeInBytes.HighPart);

    //
    // Calculate the size required for the VisitedVertices bitmap buffer.  One
    // bit is used per NumberOfVertices.  Convert the bits into bytes by
    // shifting right 3 (dividing by 8) then align it up to a 16 byte boundary.
    //

    VisitedVerticesBitmapBufferSizeInBytes.QuadPart = (
        ALIGN_UP((NumberOfVertices.QuadPart >> 3), 16)
    );

    ASSERT(!VisitedVerticesBitmapBufferSizeInBytes.HighPart);

    //
    // Calculate the sizes required for each of the arrays.  We collect them
    // into independent variables as it makes carving up the allocated buffer
    // easier down the track.
    //

    EdgesSizeInBytes = (
        ALIGN_UP_POINTER(sizeof(*Graph->Edges) * TotalNumberOfEdges.QuadPart)
    );

    NextSizeInBytes = (
        ALIGN_UP_POINTER(sizeof(*Graph->Next) * TotalNumberOfEdges.QuadPart)
    );

    FirstSizeInBytes = (
        ALIGN_UP_POINTER(sizeof(*Graph->First) * NumberOfVertices.QuadPart)
    );

    PrevSizeInBytes = (
        ALIGN_UP_POINTER(sizeof(*Graph->Prev) * TotalNumberOfEdges.QuadPart)
    );

    AssignedSizeInBytes = (
        ALIGN_UP_POINTER(sizeof(*Graph->Assigned) * NumberOfVertices.QuadPart)
    );

    //
    // Calculate the total size required for the underlying graph, such that
    // we can allocate memory via a single call to the allocator.
    //

    AllocSize.QuadPart = ALIGN_UP_POINTER(

        //
        // Account for the size of the graph structure.
        //

        sizeof(GRAPH) +

        //
        // Account for the size of the Graph->Edges array, which is double
        // sized.
        //

        EdgesSizeInBytes +

        //
        // Account for the size of the Graph->Next array; also double sized.
        //

        NextSizeInBytes +

        //
        // Account for the size of the Graph->First array.  This is sized
        // proportional to the number of vertices.
        //

        FirstSizeInBytes +

        //
        // Account for the size of the Graph->Prev array, also double sized.
        //

        PrevSizeInBytes +

        //
        // Account for Graph->Assigned array of vertices.
        //

        AssignedSizeInBytes +

        //
        // Account for the size of the bitmap buffer for Graph->DeletedEdges.
        //

        DeletedEdgesBitmapBufferSizeInBytes.QuadPart +

        //
        // Account for the size of the bitmap buffer for Graph->VisitedVertices.
        //

        VisitedVerticesBitmapBufferSizeInBytes.QuadPart

    );

    //
    // Capture the number of bitmaps here, where it's close to the two lines
    // above that indicate how many bitmaps we're dealing with.  The number
    // of bitmaps accounted for above should match this number.
    //

    NumberOfBitmaps = 2;

    //
    // Sanity check the size hasn't overflowed.
    //

    ASSERT(!AllocSize.HighPart);

    //
    // Calculate the number of pages required by each graph, then extrapolate
    // the number of guard pages and total number of pages.  We currently use
    // 4KB for the page size (i.e. we're not using large pages).
    //

    PageSize = PAGE_SIZE;
    PageShift = (USHORT)TrailingZeros(PageSize);
    NumberOfGraphs = (USHORT)Context->MaximumConcurrency;
    NumberOfPagesPerGraph = ROUND_TO_PAGES(AllocSize.LowPart) >> PageShift;
    NumberOfGuardPages = (USHORT)Context->MaximumConcurrency;
    TotalNumberOfPages = (
        (NumberOfGraphs * NumberOfPagesPerGraph) +
        NumberOfGuardPages
    );
    GraphSizeInBytesIncludingGuardPage = (
        (NumberOfPagesPerGraph * PageSize) + PageSize
    );

    //
    // Create multiple buffers separated by guard pages using a single call
    // to VirtualAllocEx().
    //

    ProcessHandle = NULL;

    Success = Rtl->CreateMultipleBuffers(Rtl,
                                         &ProcessHandle,
                                         PageSize,
                                         NumberOfGraphs,
                                         NumberOfPagesPerGraph,
                                         NULL,
                                         NULL,
                                         &UsableBufferSizeInBytesPerBuffer,
                                         &TotalBufferSizeInBytes,
                                         &BaseAddress);

    if (!Success) {
        return FALSE;
    }

    //
    // N.B. Subsequent errors must 'goto Error' at this point to ensure our
    //      cleanup logic kicks in.
    //

    //
    // Assert the sizes returned by the buffer allocation match what we're
    // expecting.
    //

    ExpectedTotalBufferSizeInBytes = TotalNumberOfPages * PageSize;
    ExpectedUsableBufferSizeInBytesPerBuffer = NumberOfPagesPerGraph * PageSize;

    ASSERT(TotalBufferSizeInBytes == ExpectedTotalBufferSizeInBytes);
    ASSERT(UsableBufferSizeInBytesPerBuffer ==
           ExpectedUsableBufferSizeInBytesPerBuffer);

    //
    // Initialize the GRAPH_INFO structure with all the sizes captured earlier.
    // (We zero it first just to ensure any of the padding fields are cleared.)
    //

    ZeroStruct(Info);

    Info.PageSize = PageSize;
    Info.AllocSize = AllocSize.QuadPart;
    Info.Context = Context;
    Info.BaseAddress = BaseAddress;
    Info.NumberOfPagesPerGraph = NumberOfPagesPerGraph;
    Info.NumberOfGraphs = NumberOfGraphs;
    Info.NumberOfBitmaps = NumberOfBitmaps;
    Info.SizeOfGraphStruct = sizeof(GRAPH);
    Info.EdgesSizeInBytes = EdgesSizeInBytes;
    Info.NextSizeInBytes = NextSizeInBytes;
    Info.FirstSizeInBytes = FirstSizeInBytes;
    Info.PrevSizeInBytes = PrevSizeInBytes;
    Info.AssignedSizeInBytes = AssignedSizeInBytes;
    Info.AllocSize = AllocSize.QuadPart;
    Info.FinalSize = UsableBufferSizeInBytesPerBuffer;

    Info.DeletedEdgesBitmapBufferSizeInBytes = (
        DeletedEdgesBitmapBufferSizeInBytes.QuadPart
    );

    Info.VisitedVerticesBitmapBufferSizeInBytes = (
        VisitedVerticesBitmapBufferSizeInBytes.QuadPart
    );

    //
    // Copy the dimensions over.
    //

    Info.NumberOfEdges = NumberOfEdges.LowPart;
    Info.TotalNumberOfEdges = TotalNumberOfEdges.LowPart;
    Info.NumberOfVertices = NumberOfVertices.LowPart;

    //
    // Set the context work callback to our worker routine, and the algo
    // context to our graph info structure.
    //

    Context->MainCallback = ProcessGraphCallbackChm01;
    Context->AlgorithmContext = &Info;

    //
    // We're ready to create threadpool work for the graph.
    //

    Buffer = (PBYTE)BaseAddress;
    Unusable = Buffer;

    for (Index = 0; Index < NumberOfGraphs; Index++) {

        //
        // Invariant check: at the top of the loop, Buffer and Unusable should
        // point to the same address (which will be the base of the current
        // graph being processed).  Assert this invariant now.
        //

        ASSERT(Buffer == Unusable);

        //
        // Carve out the graph pointer, and bump the unusable pointer past the
        // graph's pages, such that it points to the first byte of the guard
        // page.
        //

        Graph = (PGRAPH)Buffer;
        Unusable = Buffer + UsableBufferSizeInBytesPerBuffer;

        //
        // Sanity check the page alignment logic.  If we subtract 1 byte from
        // Unusable, it should reside on a different page.  Additionally, the
        // two pages should be separated by at most a single page size.
        //

        ThisPage = ALIGN_DOWN(Unusable,   PageSize);
        LastPage = ALIGN_DOWN(Unusable-1, PageSize);
        ASSERT(LastPage < ThisPage);
        ASSERT((ThisPage - LastPage) == PageSize);

        //
        // Verify the guard page is working properly by wrapping an attempt to
        // write to it in a structured exception handler that will catch the
        // access violation trap.
        //
        // N.B. We only do this if we're not actively being debugged, as the
        //      traps get dispatched to the debugger engine first as part of
        //      the "first-pass" handling logic of the kernel.
        //

        if (!IsDebuggerPresent()) {

            CaughtException = FALSE;

            TRY_PROBE_MEMORY{

                *Unusable = 1;

            } CATCH_EXCEPTION_ACCESS_VIOLATION{

                CaughtException = TRUE;

            }

            ASSERT(CaughtException);
        }

        //
        // Guard page is working properly.  Push the graph onto the context's
        // main work list head and submit the corresponding threadpool work.
        //

        InterlockedPushEntrySList(&Context->MainListHead, &Graph->ListEntry);
        SubmitThreadpoolWork(Context->MainWork);

        //
        // Advance the buffer past the graph size and guard page.  Copy the
        // same address to the Unusable variable as well, such that our top
        // of the loop invariants hold true.
        //

        Buffer += GraphSizeInBytesIncludingGuardPage;
        Unusable = Buffer;

        //
        // If our key set size is small and our maximum concurrency is large,
        // we may have already solved the graph, in which case, we can stop
        // submitting new solver attempts and just break out of the loop here.
        //

        if (!ShouldWeContinueTryingToSolveGraph(Context)) {
            break;
        }
    }

    //
    // Wait on the context's events.
    //

    Events[0] = Context->SucceededEvent;
    Events[1] = Context->CompletedEvent;
    Events[2] = Context->ShutdownEvent;
    Events[3] = Context->FailedEvent;

    WaitResult = WaitForMultipleObjects(NumberOfEvents,
                                        Events,
                                        FALSE,
                                        INFINITE);

    //
    // Ignore the wait result; determine if the graph solving was successful
    // by the finished count of the context.
    //

    Success = (Context->FinishedCount > 0);

    //
    // Destroy the buffer we created earlier.
    //
    // N.B. Although we used Rtl->CreateMultipleBuffers(), we can still free
    //      the underlying buffer via Rtl->DestroyBuffer(), as only a single
    //      VirtualAllocEx() call was dispatched for the entire buffer.
    //

    Rtl->DestroyBuffer(Rtl, ProcessHandle, &BaseAddress);

    return Success;
}

//
// The entry point into the actual per-thread solving attempts is the following
// routine.
//

_Use_decl_annotations_
VOID
ProcessGraphCallbackChm01(
    PPERFECT_HASH_TABLE_CONTEXT Context,
    PSLIST_ENTRY ListEntry
    )
{
    PRTL Rtl;
    PGRAPH Graph;
    PGRAPH_INFO Info;
    PFILL_PAGES FillPages;

    //
    // Resolve the graph base address from the list entry.  Nothing will be
    // filled in initially.
    //

    Graph = CONTAINING_RECORD(ListEntry, GRAPH, ListEntry);

    //
    // Resolve aliases.
    //

    Rtl = Context->Rtl;
    FillPages = Rtl->FillPages;

    //
    // The graph info structure will be stashed in the algo context field.
    //

    Info = (PGRAPH_INFO)Context->AlgorithmContext;

    //
    // Begin the solving loop.  InitializeGraph() generates new seed data,
    // so each loop iteration will be attempting to solve the graph uniquely.
    //

    while (ShouldWeContinueTryingToSolveGraph(Context)) {

        InitializeGraph(Info, Graph);

        if (SolveGraph(Graph)) {

            //
            // Hey, we were the ones to solve it, great!
            //

            break;
        }

        //
        // Our attempt at solving failed.  Zero all pages associated with the
        // graph and then try again with new seed data.
        //

        FillPages((PCHAR)Graph, 0, Info->NumberOfPagesPerGraph);

    }

    return;
}

_Use_decl_annotations_
VOID
InitializeGraph(
    PGRAPH_INFO Info,
    PGRAPH Graph
    )
{
    ULONG Index;
    PBYTE Buffer;
    PBYTE ExpectedBuffer;
    USHORT BitmapCount = 0;

    //
    // Obtain new seed data.
    //

    GetRandomSeedsBlocking(&Graph->Seeds);

    //
    // Carve out the backing memory structures for arrays and bitmap buffers.
    // Use the PBYTE Buffer here to make pointer arithmetic a tad easier.  We
    // initialize it to the start of the memory immediately following the graph
    // structure, and then bump it each time we carve out an array or bitmap
    // buffer.
    //

    ASSERT(sizeof(*Graph) == Info->SizeOfGraphStruct);
    Buffer = RtlOffsetToPointer(Graph, sizeof(*Graph));

    //
    // Carve out the Graph->Edges array.
    //

    Graph->Edges = (PEDGE)Buffer;
    Buffer += Info->EdgesSizeInBytes;

    //
    // Carve out the Graph->Next array.
    //

    Graph->Next = (PEDGE)Buffer;
    Buffer += Info->NextSizeInBytes;

    //
    // Carve out the Graph->First array.
    //

    Graph->First = (PVERTEX)Buffer;
    Buffer += Info->FirstSizeInBytes;

    //
    // Carve out the Graph->Prev array.
    //

    Graph->Prev = (PVERTEX)Buffer;
    Buffer += Info->PrevSizeInBytes;

    //
    // Carve out the Graph->Assigned array.
    //

    Graph->Assigned = (PVERTEX)Buffer;
    Buffer += Info->AssignedSizeInBytes;

    //
    // Carve out the bitmap buffer for Graph->DeletedEdges.
    //

    Graph->DeletedEdges.Buffer = (PULONG)Buffer;
    Graph->DeletedEdges.SizeOfBitMap = Info->TotalNumberOfEdges;
    Buffer += Info->DeletedEdgesBitmapBufferSizeInBytes;
    BitmapCount++;

    //
    // Carve out the bitmap buffer for Graph->VisitedEdges.
    //

    Graph->VisitedVertices.Buffer = (PULONG)Buffer;
    Graph->VisitedVertices.SizeOfBitMap = Info->NumberOfVertices;
    Buffer += Info->VisitedVerticesBitmapBufferSizeInBytes;
    BitmapCount++;

    //
    // Verify we visited the number of bitmaps we were expecting to visit.
    //

    ASSERT(Info->NumberOfBitmaps == BitmapCount);

    //
    // If our pointer arithmetic was correct, Buffer should match the base
    // address of the graph plus the total allocation size at this point.
    // Assert this invariant now.
    //

    ExpectedBuffer = RtlOffsetToPointer(Graph, Info->AllocSize);
    ASSERT(Buffer == ExpectedBuffer);

    //
    // Set the current thread ID and capture an attempt number from context.
    // Save the info address.
    //

    Graph->ThreadId = GetCurrentThreadId();
    Graph->Attempt = InterlockedIncrement(&Info->Context->Attempts);
    Graph->Info = Info;

    //
    // Replicate the graph dimensions.
    //

    Graph->NumberOfEdges = Info->NumberOfEdges;
    Graph->TotalNumberOfEdges = Info->TotalNumberOfEdges;
    Graph->NumberOfVertices = Info->NumberOfVertices;

    //
    // Set the context.
    //

    Graph->Context = Info->Context;

    //
    // "Empty" all of the nodes; which they've chosen to mean setting them
    // all to -1.  (Can't we use 0 here?  This seems unnecessarily inefficient.)
    //

    for (Index = 0; Index < Info->NumberOfVertices; Index++) {
        Graph->First[Index] = EMPTY;
    }

    for (Index = 0; Index < Info->TotalNumberOfEdges; Index++) {
        Graph->Next[Index] = EMPTY;
        Graph->Edges[Index] = EMPTY;
    }

    //
    // Initialization complete!
    //

    return;
}

BOOLEAN
ShouldWeContinueTryingToSolveGraph(
    PPERFECT_HASH_TABLE_CONTEXT Context
    )
{
    ULONG WaitResult;
    HANDLE Events[] = {
        Context->ShutdownEvent,
        Context->SucceededEvent,
        Context->FailedEvent,
        Context->CompletedEvent,
    };
    USHORT NumberOfEvents = ARRAYSIZE(Events);

    //
    // Fast-path exit: if the finished count is not 0, then someone has already
    // solved the solution, and we don't need to wait on any of the events.
    //

    if (Context->FinishedCount > 0) {
        return FALSE;
    }

    //
    // N.B. We should probably switch this to simply use volatile field of the
    //      context structure to indicate whether or not the context is active.
    //      WaitForMultipleObjects() on four events seems a bit... excessive.
    //

    WaitResult = WaitForMultipleObjects(NumberOfEvents,
                                        Events,
                                        FALSE,
                                        0);

    //
    // The only situation where we continue attempting to solve the graph is
    // if the result from the wait is WAIT_TIMEOUT, which indicates none of
    // the events have been set.  We treat any other situation as an indication
    // to stop processing.  (This includes wait failures and abandonment.)
    //

    return (WaitResult == WAIT_TIMEOUT ? TRUE : FALSE);
}

////////////////////////////////////////////////////////////////////////////////
// Algorithm Implementation
////////////////////////////////////////////////////////////////////////////////

//
// The guts of the CHM algorithm implementation begins here.  (Everything else
// up to this point has been scaffolding for the graph creation and threadpool
// setup.)
//

//
// The algorithm is as follows:
//
//  For each key:
//      Generate unique hash 1 (h1/v1) and hash 2 (h2/v2)
//      Add edge to graph for h1<->h2
//  Determine if graph is cyclic.  If so, restart.
//  If not, we've found a solution; perform assignment and finish up.
//

//
// Forward definitions.
//

GRAPH_ASSIGN GraphAssign;
GRAPH_EDGE_ID GraphEdgeId;
GRAPH_ADD_EDGE GraphAddEdge;
GRAPH_ITERATOR GraphIterator;
GRAPH_TRAVERSE GraphTraverse;
IS_GRAPH_ACYCLIC IsGraphAcyclic;
GRAPH_DELETE_EDGE GraphDeleteEdge;
GRAPH_NEXT_NEIGHBOR GraphNextNeighbor;
GRAPH_CONTAINS_EDGE GraphContainsEdge;
GRAPH_FIND_DEGREE1_EDGE GraphFindDegree1Edge;
GRAPH_CYCLIC_DELETE_EDGE GraphCyclicDeleteEdge;

//
// Implementations.
//

_Use_decl_annotations_
VOID
GraphAddEdge(
    PGRAPH Graph,
    VERTEX Vertex1,
    VERTEX Vertex2
    )
/*++

Routine Description:

    This routine adds an edge to the hypergraph for two vertices.  The current
    edge is determined via Graph->CurrentEdge, which is also incremented as part
    of this routine.

Arguments:

    Graph - Supplies a pointer to the graph for which the edge is to be added.

    Vertex1 - Supplies the first vertex.

    Vertex2 - Supplies the second vertex.

Return Value:

    None.

--*/
{
    EDGE Edge1;
    EDGE Edge2;

    Edge1 = Graph->CurrentEdge;
    Edge2 = Edge1 + Graph->NumberOfEdges;

#ifdef _DEBUG
    ASSERT(Vertex1 < Graph->NumberOfVertices);
    ASSERT(Vertex2 < Graph->NumberOfVertices);
    ASSERT(Edge1 < Graph->NumberOfEdges);
    ASSERT(!Graph->Flags.Shrinking);
#endif

    Graph->Next[Edge1] = Graph->First[Vertex1];
    Graph->First[Vertex1] = Edge1;
    Graph->Edges[Edge1] = Vertex2;

    Graph->Next[Edge2] = Graph->First[Vertex2];
    Graph->First[Vertex2] = Edge2;
    Graph->Edges[Edge2] = Vertex1;

    Graph->CurrentEdge++;
}

_Use_decl_annotations_
BOOLEAN
GraphFindDegree1Edge(
    PGRAPH Graph,
    VERTEX Vertex,
    PEDGE EdgePointer
    )
/*++

Routine Description:

    This routine determines if a vertex has degree 1 within the graph, and if
    so, returns the edge associated with it.

Arguments:

    Graph - Supplies a pointer to the graph.

    Vertex - Supplies the vertex for which the degree 1 test is made.

    EdgePointer - Supplies the address of a variable that receives the EDGE
        owning this vertex if it degree 1.

Return Value:

    TRUE if the vertex has degree 1, FALSE otherwise.  EdgePointer will be
    updated if TRUE is returned.

    N.B. Actually, in the CHM implementation, they seem to update the edge
         regardless if it was a degree 1 connection.  I guess we should mirror
         that behavior now too.

--*/
{
    EDGE Edge;
    EDGE Edge1;
    BOOLEAN Found = FALSE;

    //
    // Get the edge for this vertex.
    //

    Edge = Graph->First[Vertex];

    //
    // If edge is empty, we're done.
    //

    if (IsEmpty(Edge)) {
        return FALSE;
    }

    //
    // If the edge has not been deleted, capture it.
    //

    if (!IsDeletedEdge(Graph, Edge)) {
        Found = TRUE;
        *EdgePointer = Edge;
    }

    //
    // Determine if this is a degree 1 connection.
    //
    // (This seems a bit... quirky.)
    //

    while (TRUE) {

        Edge = Graph->Next[Edge];

        if (IsEmpty(Edge)) {
            break;
        }

        Edge1 = GetFirstEdgeDebug(Graph, Edge);

        if (IsDeletedEdge(Graph, Edge1)) {
            continue;
        }

        if (Found) {
            return FALSE;
        }

        //
        // We've found the first edge.
        //

        *EdgePointer = Edge;
        Found = TRUE;
    }

    return Found;
}

_Use_decl_annotations_
VOID
GraphCyclicDeleteEdge(
    PGRAPH Graph,
    VERTEX Vertex
    )
/*++

Routine Description:

    This routine deletes edges from a graph connected by vertices of degree 1.

Arguments:

    Graph - Supplies a pointer to the graph for which the edge is to be deleted.

    Vertex - Supplies the vertex for which the initial edge is obtained.

Return Value:

    None.

    N.B. If an edge is deleted, its corresponding bit will be set in the bitmap
         Graph->DeletedEdges.

--*/
{
    EDGE Edge;
    EDGE Edge1;
    EDGE Edge2;
    VERTEX Vertex1;
    VERTEX Vertex2;
    BOOLEAN IsDegree1;

    //
    // Determine if the vertex has a degree of 1, and if so, obtain the edge.
    //

    IsDegree1 = GraphFindDegree1Edge(Graph, Vertex, &Edge);

    //
    // If this isn't a degree 1 edge, there's nothing left to do.
    //

    if (!IsDegree1) {
        return;
    }

    //
    // We've found an edge of degree 1 to delete.
    //

    Vertex1 = Vertex;
    Vertex2 = 0;

    while (TRUE) {

        Edge1 = GetFirstEdgeDebug(Graph, Edge);
        RegisterEdgeDeletion(Graph, Edge);

        //
        // Find the other vertex the edge connected.
        //

        Vertex2 = Graph->Edges[Edge1];

        if (Vertex1 == Vertex2) {
            Edge2 = GetSecondEdgeDebug(Graph, Edge);
            Vertex2 = Graph->Edges[Edge2];
        }

        IsDegree1 = GraphFindDegree1Edge(Graph, Vertex2, &Edge);

        if (!IsDegree1) {
            break;
        }

        //
        // This vertex is also degree 1, so continue the inspection.
        //

        Vertex1 = Vertex2;
    }
}

_Use_decl_annotations_
BOOLEAN
IsGraphAcyclic(
    PGRAPH Graph
    )
/*++

Routine Description:

    This routine determines whether or not the graph is acyclic.  An acyclic
    graph is one where, after deletion of all edges in the graph with vertices
    of degree 1, no edges remain.

Arguments:

    Graph - Supplies a pointer to the graph to operate on.

Return Value:

    TRUE if the graph is acyclic, FALSE if it's cyclic.

--*/
{
    VERTEX Vertex;
    BOOLEAN IsAcyclic;
    ULONG NumberOfEdges;
    ULONG NumberOfVertices;
    ULONG NumberOfEdgesDeleted;
    PRTL_NUMBER_OF_SET_BITS RtlNumberOfSetBits;

    //
    // Resolve aliases.
    //

    NumberOfEdges = Graph->NumberOfEdges;
    NumberOfVertices = Graph->NumberOfVertices;
    RtlNumberOfSetBits = Graph->Context->Rtl->RtlNumberOfSetBits;

    //
    // Invariant check: we should not be shrinking prior to this point.
    //

    ASSERT(!Graph->Flags.Shrinking);

    //
    // Toggle the shrinking bit to indicate we've started edge deletion.
    //

    Graph->Flags.Shrinking = TRUE;

    //
    // Enumerate through all vertices in the graph and attempt to delete those
    // connected by edges that have degree 1.
    //

    for (Vertex = 0; Vertex < NumberOfVertices; Vertex++) {
        GraphCyclicDeleteEdge(Graph, Vertex);
    }

    //
    // As each edge of degree 1 is deleted, a bit is set in the deleted bitmap,
    // indicating the edge at that bit offset was deleted.  Thus, we can simply
    // count the number of set bits in the bitmap and compare that to the number
    // of edges in the graph.  If the values do not match, the graph is cyclic;
    // if they do match, the graph is acyclic.
    //

    NumberOfEdgesDeleted = RtlNumberOfSetBits(&Graph->DeletedEdges);

    IsAcyclic = (NumberOfEdges == NumberOfEdgesDeleted);

    //
    // Make a note that we're acyclic if applicable in the graph's flags.
    // This is checked by GraphAssign() to ensure we only operate on acyclic
    // graphs.
    //

    if (IsAcyclic) {
        Graph->Flags.IsAcyclic = TRUE;
    }

    return IsAcyclic;
}

_Use_decl_annotations_
VOID
GraphAssign(
    PGRAPH Graph
    )
/*++

Routine Description:

    This routine is called after a graph has determined to be acyclic.  It is
    responsible for walking the graph and assigning values to edges in order to
    complete the perfect hash solution.

Arguments:

    Graph - Supplies a pointer to the graph to operate on.

Return Value:

    TRUE if the graph is acyclic, FALSE if it's cyclic.

--*/
{
    VERTEX Vertex;

    //
    // Invariant check: the acyclic flag should be set.  (Indicating that
    // IsGraphAcyclic() successfully determined that, yes, the graph is
    // acyclic.)
    //

    ASSERT(Graph->Flags.IsAcyclic);

    //
    // Walk the graph and assign values.
    //

    for (Vertex = 0; Vertex < Graph->NumberOfVertices; Vertex++) {

        if (!IsVisitedVertex(Graph, Vertex)) {

            //
            // Assign an initial value of 0, then walk the subgraph.
            //

            Graph->Assigned[Vertex] = 0;
            GraphTraverse(Graph, Vertex);
        }
    }

    return;
}

_Use_decl_annotations_
GRAPH_ITERATOR
GraphNeighborsIterator(
    PGRAPH Graph,
    VERTEX Vertex
    )
/*++

Routine Description:

    For a given vertex in graph, create an iterator such that the neighboring
    vertices can be iterated over.

Arguments:

    Graph - Supplies a pointer to the graph to operate on.

    Vertex - Supplies the vertex for which the iterator will be initialized.

Return Value:

    An instance of a GRAPH_ITERATOR with the Vertex member set to the Vertex
    parameter, and the Edge member set to the first edge in the graph for the
    given vertex.

--*/
{
    GRAPH_ITERATOR Iterator;

    Iterator.Vertex = Vertex;
    Iterator.Edge = Graph->First[Vertex];

    return Iterator;
}

_Use_decl_annotations_
VERTEX
GraphNextNeighbor(
    PGRAPH Graph,
    PGRAPH_ITERATOR Iterator
    )
/*++

Routine Description:

    Return the next vertex for a given graph iterator.

Arguments:

    Graph - Supplies a pointer to the graph to operate on.

    Iterator - Supplies a pointer to the graph iterator structure to use.

Return Value:

    The neighboring vertex, or GRAPH_NO_NEIGHBOR if no vertices remain.

--*/
{
    EDGE Edge;
    VERTEX Vertex;
    VERTEX Neighbor;

    //
    // If the edge is empty, the graph iteration has finished.
    //

    Edge = Iterator->Edge;

    if (IsEmpty(Edge)) {
        return GRAPH_NO_NEIGHBOR;
    }

    //
    // Find the vertex for this edge.
    //

    Vertex = Graph->Edges[Edge];

    //
    // If the vertex matches the one in our iterator, the edge we've been
    // provided is the first edge.  Otherwise, it's the second edge.
    //

    if (Vertex == Iterator->Vertex) {

        Neighbor = Graph->Edges[Edge + Graph->NumberOfEdges];

    } else {

        Neighbor = Vertex;
    }

    //
    // Update the edge and return the neighbor.
    //

    Iterator->Edge = Graph->Next[Edge];

    return Neighbor;
}


_Use_decl_annotations_
EDGE
GraphEdgeId(
    PGRAPH Graph,
    VERTEX Vertex1,
    VERTEX Vertex2
    )
/*++

Routine Description:

    Generates an ID for two vertices as part of the assignment step.

Arguments:

    Graph - Supplies a pointer to the graph for which the edge is to be added.

    Vertex1 - Supplies the first vertex.

    Vertex2 - Supplies the second vertex.

Return Value:

    An EDGE value.

--*/
{
    EDGE Edge;
    EDGE EdgeId;

    Edge = Graph->First[Vertex1];

    ASSERT(!IsEmpty(Edge));

    if (GraphCheckEdge(Graph, Edge, Vertex1, Vertex2)) {

        EdgeId = GetFirstEdgeDebug(Graph, Edge);

    } else {

        do {

            Edge = Graph->Next[Edge];
            ASSERT(!IsEmpty(Edge));

        } while (!GraphCheckEdge(Graph, Edge, Vertex1, Vertex2));

        EdgeId = GetFirstEdgeDebug(Graph, Edge);
    }

    return EdgeId;
}

_Use_decl_annotations_
VOID
GraphTraverse(
    PGRAPH Graph,
    VERTEX Vertex
    )
/*++

Routine Description:

    This routine is called as part of graph assignment.

Arguments:

    Graph - Supplies a pointer to the graph to operate on.

    Vertex - Supplies the vertex to traverse.

Return Value:

    None.

--*/
{
    ULONG Id1;
    ULONG Id2;
    ULONG Id3;
    VERTEX Neighbor;
    GRAPH_ITERATOR Iterator;

    //
    // Register the vertex as visited.
    //

    RegisterVertexVisit(Graph, Vertex);

    //
    // Initialize a graph iterator for visiting neighbors.
    //

    Iterator = GraphNeighborsIterator(Graph, Vertex);

    while (TRUE) {

        Neighbor = GraphNextNeighbor(Graph, &Iterator);

        if (IsNeighborEmpty(Neighbor)) {
            break;
        }

        //
        // If the neighbor has already been visited, skip it.
        //

        if (IsVisitedVertex(Graph, Neighbor)) {
            continue;
        }

        //
        // Construct the unique ID for this particular visit.  We break it out
        // into three distinct steps in order to assist with debugging.
        //

        Id1 = GraphEdgeId(Graph, Vertex, Neighbor);
        Id2 = Graph->Assigned[Vertex];
        Id3 = Id1 - Id2;

        Graph->Assigned[Neighbor] = Id3;

        //
        // Recursively traverse the neighbor.
        //

        GraphTraverse(Graph, Neighbor);
    }
}


_Use_decl_annotations_
BOOLEAN
SolveGraph(
    _In_ PGRAPH Graph
    )
{
    EDGE Index;
    EDGE Key;
    PEDGE Keys;
    VERTEX Vertex1;
    VERTEX Vertex2;
    PGRAPH_INFO Info;
    ULONG NumberOfEdges;
    PPERFECT_HASH_TABLE Table;
    PPERFECT_HASH_TABLE_CONTEXT Context;

    Info = Graph->Info;
    Context = Info->Context;
    Table = Context->Table;
    NumberOfEdges = Graph->NumberOfEdges;
    Keys = (PEDGE)Table->Keys->BaseAddress;

    for (Index = 0; Index < NumberOfEdges; Index++) {
        Key = Keys[Index];

        if (!HashKey(Graph, Key, &Vertex1, &Vertex2)) {

            //
            // Failed to hash the key to two unique vertices.
            //

            return FALSE;
        }

        GraphAddEdge(Graph, Vertex1, Vertex2);
    }

    if (!IsGraphAcyclic(Graph)) {

        //
        // Failed to create an acyclic graph.
        //

        return FALSE;
    }

    //
    // We created an acyclic graph.  Increment the finished count; if the value
    // is 1, we're the winning thread.  Continue with graph assignment.
    // Otherwise, just return TRUE immediately and let the other thread win.
    //

    if (InterlockedIncrement64(&Context->FinishedCount) != 1) {

        //
        // Some other thread beat us.  Nothing left to do.
        //

        return TRUE;
    }

    //
    // We created an acyclic graph.  Perform the assignment step.
    //

    GraphAssign(Graph);

    //
    // XXX TODO: persist the graph.
    //


    //
    // Set the succeeded and completed events, then submit the finished work
    // item to the threadpool, such that the necessary cleanups can take place
    // once all our worker threads have completed.
    //

    SetEvent(Context->SucceededEvent);
    SetEvent(Context->CompletedEvent);
    SubmitThreadpoolWork(Context->FinishedWork);

    return TRUE;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
