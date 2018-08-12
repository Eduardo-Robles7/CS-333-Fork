// Create a new process copying p as the parent.
// Sets up stack to return as if from system call.
// Caller must set state of returned proc to RUNNABLE.
int
fork(void)
{
  int i, pid;
  struct proc *np;

  // Allocate process.
  if((np = allocproc()) == 0)
    return -1;

  // Copy process state from p.
  if((np->pgdir = copyuvm(proc->pgdir, proc->sz)) == 0)
  {
    kfree(np->kstack);
    np->kstack = 0;

    //remove from embryo list
    #ifdef CS333_P3P4
    acquire(&ptable.lock);
    removeFromStateList(&ptable.pLists.embryo,np);
    assertState(np,EMBRYO);
    #endif

    //add to the free list
    np->state = UNUSED;
    #ifdef CS333_P3P4
    addToStateListHead(&ptable.pLists.free,np);
    release(&ptable.lock);
    #endif
    return -1;
  }
  np->sz = proc->sz;
  np->parent = proc;
  *np->tf = *proc->tf;

  #ifdef CS333_P2
  //copy UID AND GID
  np->uid = proc->uid;
  np->gid = proc->gid;
  #endif

  // Clear %eax so that fork returns 0 in the child.
  np->tf->eax = 0;

  for(i = 0; i < NOFILE; i++)
    if(proc->ofile[i])
      np->ofile[i] = filedup(proc->ofile[i]);
  np->cwd = idup(proc->cwd);

  safestrcpy(np->name, proc->name, sizeof(proc->name));
 
  pid = np->pid;

  // lock to force the compiler to emit the np->state write last.
  acquire(&ptable.lock);

  //remove from embryo
  #ifdef CS333_P3P4
  removeFromStateList(&ptable.pLists.embryo,np);
  assertState(np,EMBRYO);
  #endif

  //add to the ready list
  np->state = RUNNABLE;
  #ifdef CS333_P3P4
  addToStateListEnd(&(ptable.pLists.ready[np->priority]),np);
  #endif

  release(&ptable.lock);
  return pid;
}