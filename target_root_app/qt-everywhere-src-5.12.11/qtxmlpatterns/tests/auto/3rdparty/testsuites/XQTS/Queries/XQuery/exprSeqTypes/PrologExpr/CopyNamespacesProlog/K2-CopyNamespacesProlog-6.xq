(:*******************************************************:)
(: Test: K2-CopyNamespacesProlog-6                       :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Check in-scope bindings of constructed nodes. :)
(:*******************************************************:)
declare namespace ns = "http://example.com/";
<e xmlns:appearsUnused="http://example.com/">
{
    <b>
        <appearsUnused:c/>
    </b>
}
</e>/(for $n in (., b, b/ns:c),
          $i in in-scope-prefixes($n)
      order by $i
      return ($i, '|'))