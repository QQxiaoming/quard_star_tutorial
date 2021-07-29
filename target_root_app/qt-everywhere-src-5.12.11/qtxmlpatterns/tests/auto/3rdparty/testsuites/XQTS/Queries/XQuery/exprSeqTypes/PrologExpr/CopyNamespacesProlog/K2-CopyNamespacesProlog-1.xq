(:*******************************************************:)
(: Test: K2-CopyNamespacesProlog-1                       :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Use no-preserve, inherit with the default namespace. :)
(:*******************************************************:)
declare copy-namespaces no-preserve, inherit;
<doc>
    <a>{<b xmlns:p="http://example.com/"/>}</a>
    <a><b xmlns:p="http://example.com/"/></a>
</doc>
