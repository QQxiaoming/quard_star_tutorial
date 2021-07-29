(:*******************************************************:)
(: Test: K2-CopyNamespacesProlog-8                       :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Copy an element with no children.            :)
(:*******************************************************:)
declare copy-namespaces no-preserve,inherit;
<a>
   {
       <b>{()}</b>
   }
</a>
