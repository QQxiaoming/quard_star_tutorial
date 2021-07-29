(:*******************************************************:)
(: Test: K2-OrderbyExprWithout-4                         :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Use an absolute, invalid collation.          :)
(:*******************************************************:)
let $i as xs:integer* := (1, 2, 3)
              order by 1 collation "http:\\invalid%>URI\someURI"
              return $i