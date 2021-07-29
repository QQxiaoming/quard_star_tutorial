(:*******************************************************:)
(: Test: K2-CopyNamespacesProlog-5                       :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Have a namespace which is used further down. :)
(:*******************************************************:)
declare copy-namespaces no-preserve, no-inherit;
<e xmlns:appearsUnused="http://example.com/">
    {
    <b>
        <appearsUnused:c/>
    </b>
    }
</e>