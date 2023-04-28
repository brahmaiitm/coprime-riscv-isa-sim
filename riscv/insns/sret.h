require_extension('S');
reg_t prev_hstatus = STATE.hstatus->read();
if (STATE.v) {
  if (STATE.prv == PRV_U || get_field(prev_hstatus, HSTATUS_VTSR))
    require_novirt();
} else {
  require_privilege(get_field(STATE.mstatus->read(), MSTATUS_TSR) ? PRV_M : PRV_S);
}
/ Trap in M-mode to decrypt registers
if(prev_prv == PRV_U){
    // printf("sret\n");
    reg_t next_pc = p->get_state()->mtvec;
    set_pc_and_serialize(next_pc);
 
    p->set_csr(CSR_MEPC, p->get_csr(CSR_SEPC));
    p->set_csr(CSR_MCAUSE, 15);
    p->set_csr(CSR_MTVAL, 0);
 
    s = set_field(s, MSTATUS_MPIE, get_field(s, MSTATUS_MIE));
    s = set_field(s, MSTATUS_MPP, PRV_U);
    s = set_field(s, MSTATUS_MIE, 0);
    p->set_csr(CSR_MSTATUS, s);
    p->set_privilege(PRV_M);
}
else {
    reg_t next_pc = p->get_state()->sepc->read();
    set_pc_and_serialize(next_pc);
    reg_t s = STATE.sstatus->read();
    reg_t prev_prv = get_field(s, MSTATUS_SPP);
    s = set_field(s, MSTATUS_SIE, get_field(s, MSTATUS_SPIE));
    s = set_field(s, MSTATUS_SPIE, 1);
    s = set_field(s, MSTATUS_SPP, PRV_U);
    STATE.sstatus->write(s);
    p->set_privilege(prev_prv);
    if (!STATE.v) {
      if (p->extension_enabled('H')) {
        reg_t prev_virt = get_field(prev_hstatus, HSTATUS_SPV);
        p->set_virt(prev_virt);
        reg_t new_hstatus = set_field(prev_hstatus, HSTATUS_SPV, 0);
        STATE.hstatus->write(new_hstatus);
      }
   
     STATE.mstatus->write(set_field(STATE.mstatus->read(), MSTATUS_MPRV, 0));
    }
}
