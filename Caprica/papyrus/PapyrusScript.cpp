#include <papyrus/PapyrusScript.h>

#ifdef _WIN32
#include <lmcons.h>
#include <Windows.h>
#else
#include <unistd.h>
#include <cstdlib>
#endif

#include <common/CapricaConfig.h>
#include <common/EngineLimits.h>

namespace caprica { namespace papyrus {

pex::PexFile* PapyrusScript::buildPex(CapricaReportingContext& repCtx) const {
  auto alloc = new allocators::ChainedPool(1024 * 4);
  auto pex = alloc->make<pex::PexFile>(alloc);
  pex->setGameAndVersion(conf::Papyrus::game);
  if (conf::CodeGeneration::emitDebugInfo) {
    pex->debugInfo = alloc->make<pex::PexDebugInfo>();
    pex->debugInfo->modificationTime = lastModificationTime;
  }
  pex->compilationTime = time(nullptr);
  pex->sourceFileName = pex->alloc->allocateString(sourceFileName);

  static std::string computerName = []() -> std::string {
#ifdef _WIN32
    char compNameBuf[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD compNameBufLength = sizeof(compNameBuf);
    if (!GetComputerNameA(compNameBuf, &compNameBufLength))
      CapricaReportingContext::logicalFatal("Failed to get the computer name!");
    return std::string(compNameBuf, compNameBufLength);
#else
    char compNameBuf[256];
    if (gethostname(compNameBuf, sizeof(compNameBuf)) != 0)
      CapricaReportingContext::logicalFatal("Failed to get the computer name!");
    compNameBuf[sizeof(compNameBuf) - 1] = '\0';
    return std::string(compNameBuf);
#endif
  }();
  pex->computerName = computerName;

  static std::string userName = []() -> std::string {
#ifdef _WIN32
    char userNameBuf[UNLEN + 1];
    DWORD userNameBufLength = sizeof(userNameBuf);
    if (!GetUserNameA(userNameBuf, &userNameBufLength))
      CapricaReportingContext::logicalFatal("Failed to get the user name!");
    if (userNameBufLength > 0)
      userNameBufLength--;
    return std::string(userNameBuf, userNameBufLength);
#else
    const char* u = getenv("USER");
    if (!u)
      u = getenv("USERNAME");
    return std::string(u ? u : "unknown");
#endif
  }();
  pex->userName = userName;

  for (auto o : objects)
    o->buildPex(repCtx, pex);

  if (objects.size()) {
    EngineLimits::checkLimit(repCtx,
                             objects.front()->location,
                             EngineLimits::Type::PexFile_UserFlagCount,
                             pex->getUserFlagCount());
  }
  return pex;
}

}}
