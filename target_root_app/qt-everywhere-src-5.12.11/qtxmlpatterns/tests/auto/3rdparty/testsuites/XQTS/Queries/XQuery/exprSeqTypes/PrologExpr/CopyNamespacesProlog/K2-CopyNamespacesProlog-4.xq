(:*******************************************************:)
(: Test: K2-CopyNamespacesProlog-4                       :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Verify that the default element namespace is not touched when using no-inherit. :)
(:*******************************************************:)
declare copy-namespaces preserve, no-inherit;
<e xmlns="http://example.com/">
{
    <b/>
}
</e>