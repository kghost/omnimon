#include <chrono>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "../CpuStat.hpp"
#include "../LoadAvg.hpp"
#include "../MemInfo.hpp"
#include "../SchedStat.hpp"
#include "../SysInfo.hpp"
#include "../Uptime.hpp"
#include "../VmStat.hpp"

void PrintHeader(const std::string& title) {
  std::cout << "\n" << std::string(80, '=') << "\n";
  std::cout << "  " << title << "\n";
  std::cout << std::string(80, '=') << "\n";
}

int main() {
  backend::system::SysInfo sysInfo;

  // Initialize and update all modules
  auto* uptime = sysInfo.GetUptime();
  auto* loadAvg = sysInfo.GetLoadAvg();
  auto* cpuStat = sysInfo.GetCpuStat();
  auto* schedStat = sysInfo.GetSchedStat();
  auto* memInfo = sysInfo.GetMemInfo();
  auto* vmStat = sysInfo.GetVmStat();

  uptime->Update();
  loadAvg->Update();
  cpuStat->Update();
  schedStat->Update();
  memInfo->Update();
  vmStat->Update();

  std::cout << std::left;

  // 1. System Constants
  PrintHeader("SYSTEM CONSTANTS");
  std::cout << std::setw(30) << "System Jiffies (Clock Ticks):" << sysInfo.GetSystemJiffies()->GetValue() << "\n";
  std::cout << std::setw(30) << "Total Memory (Pages):" << sysInfo.GetTotalMem()->GetValue() << "\n";

  // 2. Uptime & Load Average
  PrintHeader("UPTIME & LOAD AVERAGE");
  std::cout << std::setw(30) << "System Uptime:" << uptime->GetUptime()->GetValue().count() << " seconds\n";
  std::cout << std::setw(30) << "System Idle Time:" << uptime->GetIdle()->GetValue().count() << " seconds\n";
  std::cout << std::setw(30) << "Load Average (1m):" << loadAvg->GetLoad1()->GetValue() << "\n";
  std::cout << std::setw(30) << "Load Average (5m):" << loadAvg->GetLoad5()->GetValue() << "\n";
  std::cout << std::setw(30) << "Load Average (15m):" << loadAvg->GetLoad15()->GetValue() << "\n";
  std::cout << std::setw(30) << "Runnable Processes:" << loadAvg->GetRunnable()->GetValue() << "\n";
  std::cout << std::setw(30) << "Total Tasks:" << loadAvg->GetTotalTasks()->GetValue() << "\n";
  std::cout << std::setw(30) << "Last PID:" << loadAvg->GetLastPid()->GetValue() << "\n";

  // 3. CPU Statistics
  PrintHeader("CPU STATISTICS");
  std::cout << std::setw(8) << "CPU"
            << std::setw(10) << "User"
            << std::setw(10) << "Nice"
            << std::setw(10) << "System"
            << std::setw(10) << "Idle"
            << std::setw(10) << "IOWait"
            << std::setw(10) << "IRQ"
            << std::setw(10) << "SoftIRQ"
            << std::setw(10) << "Steal"
            << std::setw(10) << "Guest"
            << "GuestNice\n";
  std::cout << std::string(105, '-') << "\n";

  auto printCpuData = [](const std::string& name, const backend::system::CpuStat::CpuData& data) {
    std::cout << std::setw(8) << name
              << std::setw(10) << data.user->GetValue()
              << std::setw(10) << data.nice->GetValue()
              << std::setw(10) << data.system->GetValue()
              << std::setw(10) << data.idle->GetValue()
              << std::setw(10) << data.iowait->GetValue()
              << std::setw(10) << data.irq->GetValue()
              << std::setw(10) << data.softirq->GetValue()
              << std::setw(10) << data.steal->GetValue()
              << std::setw(10) << data.guest->GetValue()
              << data.guest_nice->GetValue() << "\n";
  };

  printCpuData("total", cpuStat->GetTotal());
  for (const auto& cpu : cpuStat->GetCpus()) {
    printCpuData(cpu.name, cpu);
  }

  // 4. CPU Scheduler Statistics
  PrintHeader("CPU SCHEDULER STATISTICS");
  std::cout << std::setw(8) << "CPU"
            << std::setw(12) << "Yield Count"
            << std::setw(12) << "Yield Exp"
            << std::setw(12) << "Sched Count"
            << std::setw(12) << "Sched Idle"
            << std::setw(12) << "TTWU Count"
            << std::setw(12) << "TTWU Local"
            << std::setw(20) << "RQ CPU Time (ns)"
            << std::setw(20) << "Run Delay (ns)"
            << "Timeslices\n";
  std::cout << std::string(135, '-') << "\n";

  int cpuIdx = 0;
  for (const auto& cpu : schedStat->GetCpuStats()) {
    std::cout << "cpu" << std::setw(5) << cpuIdx++
              << std::setw(12) << cpu.yld_count->GetValue()
              << std::setw(12) << cpu.yld_exp->GetValue()
              << std::setw(12) << cpu.sched_count->GetValue()
              << std::setw(12) << cpu.sched_goidle->GetValue()
              << std::setw(12) << cpu.ttwu_count->GetValue()
              << std::setw(12) << cpu.ttwu_local->GetValue()
              << std::setw(20) << cpu.rq_cpu_time->GetValue()
              << std::setw(20) << cpu.run_delay->GetValue()
              << cpu.pcpu_run_sched_info->GetValue() << "\n";
  }

  // 5. Memory Info
  PrintHeader("MEMORY INFO");
  auto printMemField = [](const std::string& name, const std::shared_ptr<backend::metrics::SharedGauge>& gauge) {
    if (gauge) {
      std::cout << std::setw(30) << (name + ":") << gauge->GetValue() << " kB\n";
    }
  };

  printMemField("MemTotal", memInfo->GetMemTotal());
  printMemField("MemFree", memInfo->GetMemFree());
  printMemField("MemAvailable", memInfo->GetMemAvailable());
  printMemField("Buffers", memInfo->GetBuffers());
  printMemField("Cached", memInfo->GetCached());
  printMemField("SwapCached", memInfo->GetSwapCached());
  printMemField("Active", memInfo->GetActive());
  printMemField("Inactive", memInfo->GetInactive());
  printMemField("Active(anon)", memInfo->GetActiveAnon());
  printMemField("Inactive(anon)", memInfo->GetInactiveAnon());
  printMemField("Active(file)", memInfo->GetActiveFile());
  printMemField("Inactive(file)", memInfo->GetInactiveFile());
  printMemField("Unevictable", memInfo->GetUnevictable());
  printMemField("Mlocked", memInfo->GetMlocked());
  printMemField("SwapTotal", memInfo->GetSwapTotal());
  printMemField("SwapFree", memInfo->GetSwapFree());
  printMemField("Dirty", memInfo->GetDirty());
  printMemField("Writeback", memInfo->GetWriteback());
  printMemField("AnonPages", memInfo->GetAnonPages());
  printMemField("Mapped", memInfo->GetMapped());
  printMemField("Shmem", memInfo->GetShmem());
  printMemField("Slab", memInfo->GetSlab());
  printMemField("SReclaimable", memInfo->GetSReclaimable());
  printMemField("SUnreclaim", memInfo->GetSUnreclaim());
  printMemField("KernelStack", memInfo->GetKernelStack());
  printMemField("PageTables", memInfo->GetPageTables());
  printMemField("Committed_AS", memInfo->GetCommittedAs());
  printMemField("VmallocTotal", memInfo->GetVmallocTotal());
  printMemField("VmallocUsed", memInfo->GetVmallocUsed());
  printMemField("VmallocChunk", memInfo->GetVmallocChunk());

  // 6. Virtual Memory Statistics
  PrintHeader("VIRTUAL MEMORY STATISTICS (VMSTAT)");
  auto printVmField = [](const std::string& name, const std::shared_ptr<backend::metrics::SharedGauge>& gauge) {
    if (gauge) {
      std::cout << std::setw(30) << (name + ":") << gauge->GetValue() << "\n";
    }
  };

  printVmField("nr_free_pages", vmStat->GetNrFreePages());
  printVmField("nr_inactive_anon", vmStat->GetNrInactiveAnon());
  printVmField("nr_active_anon", vmStat->GetNrActiveAnon());
  printVmField("nr_inactive_file", vmStat->GetNrInactiveFile());
  printVmField("nr_active_file", vmStat->GetNrActiveFile());
  printVmField("nr_unevictable", vmStat->GetNrUnevictable());
  printVmField("nr_dirty", vmStat->GetNrDirty());
  printVmField("nr_writeback", vmStat->GetNrWriteback());
  printVmField("nr_anon_pages", vmStat->GetNrAnonPages());
  printVmField("nr_mapped", vmStat->GetNrMapped());
  printVmField("nr_file_pages", vmStat->GetNrFilePages());
  printVmField("nr_slab_reclaimable", vmStat->GetNrSlabReclaimable());
  printVmField("nr_slab_unreclaimable", vmStat->GetNrSlabUnreclaimable());
  printVmField("pgpgin", vmStat->GetPgpgin());
  printVmField("pgpgout", vmStat->GetPgpgout());
  printVmField("pswpin", vmStat->GetPswpin());
  printVmField("pswpout", vmStat->GetPswpout());
  printVmField("pgfault", vmStat->GetPgfault());
  printVmField("pgmajfault", vmStat->GetPgmajfault());

  std::cout << "\n" << std::string(80, '=') << "\n";
  std::cout << "  Finished dumping all system metrics.\n";
  std::cout << std::string(80, '=') << std::endl;

  return 0;
}
