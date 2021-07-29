(:*******************************************************:)
(: Test: K2-CopyNamespacesProlog-7                       :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Have an unused prefix further down the hierarchy. :)
(:*******************************************************:)
declare copy-namespaces no-preserve, no-inherit;
<e>
    {
    <b>
        <c xmlns:unused="http://example.com"/>
    </b>
    }
</e>