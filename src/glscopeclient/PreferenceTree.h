/***********************************************************************************************************************
*                                                                                                                      *
* glscopeclient                                                                                                        *
*                                                                                                                      *
* Copyright (c) 2012-2020 Andrew D. Zonenberg                                                                          *
* All rights reserved.                                                                                                 *
*                                                                                                                      *
* Redistribution and use in source and binary forms, with or without modification, are permitted provided that the     *
* following conditions are met:                                                                                        *
*                                                                                                                      *
*    * Redistributions of source code must retain the above copyright notice, this list of conditions, and the         *
*      following disclaimer.                                                                                           *
*                                                                                                                      *
*    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the       *
*      following disclaimer in the documentation and/or other materials provided with the distribution.                *
*                                                                                                                      *
*    * Neither the name of the author nor the names of any contributors may be used to endorse or promote products     *
*      derived from this software without specific prior written permission.                                           *
*                                                                                                                      *
* THIS SOFTWARE IS PROVIDED BY THE AUTHORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED   *
* TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL *
* THE AUTHORS BE HELD LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES        *
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR       *
* BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT *
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE       *
* POSSIBILITY OF SUCH DAMAGE.                                                                                          *
*                                                                                                                      *
***********************************************************************************************************************/

/**
	@file
	@author Katharina B.
	@brief  Data structure modeling a hierachical property tree
 */

#ifndef PreferenceTree_h
#define PreferenceTree_h

#include <string>
#include <vector>

#include "Preference.h"

namespace YAML
{
    class Node;
}

enum class PreferenceTreeNodeType
{
    Preference,
    Category
};

class PreferenceCategory;

namespace internal
{
    class PreferencePath
    {
        public:
            explicit PreferencePath(const std::string& path);

        protected:
            PreferencePath(std::vector<std::string> segments);

        public:
            PreferencePath NextLevel() const;
            std::size_t GetLength() const;
            const std::string& GetCurrentSegment() const;

        protected:
            std::vector<std::string> m_segments;
    };

    class PreferenceTreeNodeBase
    {
    public:
        PreferenceTreeNodeBase(PreferenceTreeNodeType type, std::string identifier)
            : m_identifier{ std::move(identifier) }, m_type{ type }
        {
        }

        virtual ~PreferenceTreeNodeBase()
        {
        }

    public:
        // Disallow copy
        PreferenceTreeNodeBase(const PreferenceTreeNodeBase&) = delete;
        PreferenceTreeNodeBase(PreferenceTreeNodeBase&&) = default;

        PreferenceTreeNodeBase& operator=(const PreferenceTreeNodeBase&) = delete;
        PreferenceTreeNodeBase& operator=(PreferenceTreeNodeBase&&) = default;

    public:
        virtual void ToYAML(YAML::Node& node) const = 0;
        virtual void FromYAML(const YAML::Node& node) = 0;
        virtual Preference& GetLeaf(const PreferencePath& path) = 0;
        virtual bool IsVisible() const = 0;

    public:
        const std::string& GetIdentifier() const;
        PreferenceTreeNodeType GetType() const;
        bool IsCategory() const;
        bool IsPreference() const;
        PreferenceCategory& AsCategory();
        Preference& AsPreference();


    protected:
        std::string m_identifier;       //< The identifier of this node
        PreferenceTreeNodeType m_type;  //< What particular type this node has
    };

    class PreferenceHolder
        : public PreferenceTreeNodeBase
    {
    public:
        PreferenceHolder(Preference pref);

    public:
        virtual void ToYAML(YAML::Node& node) const;
        virtual void FromYAML(const YAML::Node& node);
        virtual Preference& GetLeaf(const PreferencePath& path);
        Preference& Get();
        const Preference& Get() const;
        virtual bool IsVisible() const;

    protected:
        Preference m_pref;
    };
}

class PreferenceCategory
    : public internal::PreferenceTreeNodeBase
{
protected:
    using map_type = std::map<std::string, std::unique_ptr<internal::PreferenceTreeNodeBase>>;
    using seq_type = std::vector<std::string>;

public:
    PreferenceCategory(std::string identifier);

public:
    Preference& GetLeaf(const std::string& path);
    const Preference& GetLeaf(const std::string& path) const;
    virtual void ToYAML(YAML::Node& node) const;
    virtual void FromYAML(const YAML::Node& node);
    virtual Preference& GetLeaf(const internal::PreferencePath& path);
    void AddPreference(Preference pref);
    void AddPreference(impl::PreferenceBuilder&& pref);
    PreferenceCategory& AddCategory(std::string identifier);
    map_type& GetChildren();
    const seq_type& GetOrdering() const;
    virtual bool IsVisible() const;

protected:
    map_type m_children;
    seq_type m_ordering;
};

#endif // PreferenceTree_h
