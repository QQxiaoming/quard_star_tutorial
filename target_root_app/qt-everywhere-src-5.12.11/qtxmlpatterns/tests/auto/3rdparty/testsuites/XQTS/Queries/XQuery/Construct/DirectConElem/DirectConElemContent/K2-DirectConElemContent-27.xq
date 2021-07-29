(:*******************************************************:)
(: Test: K2-DirectConElemContent-27                      :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Many CDATA sections.                         :)
(:*******************************************************:)
string(<elem><![CDATA[str]]>str<![CDATA[str]]><![CDATA[str]]><![CDATA[str]]>strstr{   "str", "str", "strstr", "str"}strstr<![CDATA[str]]>s<?target str?>tr</elem>) eq "strstrstrstrstrstrstrstr str strstr strstrstrstrstr"