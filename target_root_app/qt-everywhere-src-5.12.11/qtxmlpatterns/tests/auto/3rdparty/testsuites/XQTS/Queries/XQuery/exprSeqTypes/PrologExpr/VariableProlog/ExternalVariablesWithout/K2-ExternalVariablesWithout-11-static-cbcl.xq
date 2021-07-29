(:*******************************************************:)
(: Test: K2-ExternalVariablesWithout-11                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-10-03T14:53:33+01:00                       :)
(: Purpose: Description ensure node identity is handled through variables that has cardinality zero or more. :)
(:*******************************************************:)
declare variable $a as attribute()* := (attribute name1 {()}, attribute name2 {()}, attribute name3 {()});
declare variable $b as attribute()* := (attribute name1 {()}, attribute name2 {()}, attribute name3 {()});
$a/(let $p := position()
    return . is exactly-one($b[$p]))
