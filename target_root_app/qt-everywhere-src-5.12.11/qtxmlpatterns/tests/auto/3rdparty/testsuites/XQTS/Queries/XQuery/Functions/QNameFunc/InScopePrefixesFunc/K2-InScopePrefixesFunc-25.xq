(:*******************************************************:)
(: Test: K2-InScopePrefixesFunc-25                       :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Check the in-scope namespaces of two nodes.  :)
(:*******************************************************:)
declare namespace a = "http://example.com/1";
declare namespace b = "http://example.com/2";
declare namespace unused = "http://example.com/3";
declare namespace unused2 = "http://example.com/4";
<unused:e/>[2],
<e a:n1="content" b:n1="content">
    <a:n1/>
</e>/(for $i in in-scope-prefixes(.)
      order by $i
      return $i,
      '|',
      for $i in a:n1/in-scope-prefixes(.)
      order by $i
      return $i)