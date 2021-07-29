(: Name: Parenexpr-11 :)
(: Written by: Andreas Behm :)
(: Description: FLWOR expression :)

(for $x in (1) where (fn:true()) order by ($x) return ($x))
