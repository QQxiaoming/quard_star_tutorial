(:*******************************************************:)
(: Test: K2-NodeNamespaceURIFunc-2                       :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Extract the namespace URI from a comment node. :)
(:*******************************************************:)
<n xmlns="http://example.com">
    {namespace-uri(<?target data?>) eq ""}
</n>