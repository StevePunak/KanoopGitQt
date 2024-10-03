#include "submodule.h"

#include <gitexception.h>
#include <repository.h>

using namespace GIT;

Submodule::Submodule(Repository* repo, const QString& name, const QString& path, const QString& url) :
    GitEntity(SubmoduleEntity, repo),
    _name(name), _path(path), _url(url)
{
    try
    {
        SubmoduleHandle handle = createHandle();
        throwIfTrue(handle.isNull());
        _headCommitId = ObjectId(git_submodule_head_id(handle.value()));
        _indexCommitId = ObjectId(git_submodule_index_id(handle.value()));
        _workdirCommitId = ObjectId(git_submodule_wd_id(handle.value()));
        _fetchRecurseSumbmodulesRule = (SubmoduleRecurse)git_submodule_fetch_recurse_submodules(handle.value());
        _ignoreRule = (SubmoduleIgnore)git_submodule_ignore(handle.value());
        _updateRule = (SubmoduleUpdate)git_submodule_update_strategy(handle.value());
        handle.dispose();
    }
    catch(const GitException&)
    {
    }
}

bool Submodule::isWorkdirInitialized() const
{
    return (status() & WorkDirUninitialized) == 0;
}

bool Submodule::initialize(bool overwrite)
{
    bool result = false;
    try
    {
        SubmoduleHandle handle = createHandle();
        throwIfTrue(handle.isNull());
        throwOnError(git_submodule_init(handle.value(), overwrite));
        result = true;
    }
    catch(const GitException&)
    {
    }
    return result;
}

Repository* Submodule::open()
{
    Repository* repo = nullptr;
    try
    {
        SubmoduleHandle handle = createHandle();
        throwIfTrue(handle.isNull());
        git_repository* repoHandle = nullptr;
        throwOnError(git_submodule_open(&repoHandle, handle.value()));
        repo = new Repository(repoHandle);
    }
    catch(const GitException&)
    {
    }
    return repo;
}

Repository* Submodule::clone()
{
    Repository* repo = nullptr;
    try
    {
        SubmoduleHandle handle = createHandle();
        throwIfTrue(handle.isNull());
        git_repository* repoHandle = nullptr;
        git_submodule_update_options options = GIT_SUBMODULE_UPDATE_OPTIONS_INIT;
        throwOnError(git_submodule_clone(&repoHandle, handle.value(), &options));
        repo = new Repository(repoHandle);
    }
    catch(const GitException&)
    {
    }
    return repo;
}

bool Submodule::update(bool initialize)
{
    bool result = false;
    try
    {
        SubmoduleHandle handle = createHandle();
        throwIfTrue(handle.isNull());
        git_submodule_update_options options = GIT_SUBMODULE_UPDATE_OPTIONS_INIT;
        throwOnError(git_submodule_update(handle.value(), initialize, &options));
        result = true;
    }
    catch(const GitException&)
    {
    }
    return result;
}

Submodule::SubmoduleStatuses Submodule::status() const
{
    SubmoduleStatuses status = Unmodified;
    unsigned int stat;
    if(git_submodule_status(&stat, repository()->handle().value(), _name.toUtf8().constData(), (git_submodule_ignore_t)GitSubmoduleIgnore::Unspecified) == 0) {
        status = (SubmoduleStatuses)stat;
    }
    return status;
}

SubmoduleHandle Submodule::createHandle() const
{
    SubmoduleHandle handle;
    git_submodule* sub = nullptr;
    if(git_submodule_lookup(&sub, repository()->handle().value(), _name.toUtf8().constData()) == 0) {
        handle = SubmoduleHandle(sub);
    }
    return handle;
}
