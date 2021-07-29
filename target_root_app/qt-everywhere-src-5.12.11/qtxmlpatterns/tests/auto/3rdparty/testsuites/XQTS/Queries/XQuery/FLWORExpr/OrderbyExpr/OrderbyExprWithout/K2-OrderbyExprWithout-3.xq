(:*******************************************************:)
(: Test: K2-OrderbyExprWithout-3                         :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Use a relative, unknown collation.           :)
(:*******************************************************:)
declare base-uri "http://www.w3.org/2005/xpath-functions/";
              let $i as xs:integer* := (1, 2, 3)
              order by 1 collation "collation/"
              return $i