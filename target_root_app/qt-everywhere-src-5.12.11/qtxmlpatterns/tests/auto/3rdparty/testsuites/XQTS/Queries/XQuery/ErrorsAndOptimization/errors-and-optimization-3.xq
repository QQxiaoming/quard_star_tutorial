(: Name: errors-and-optimization-3.xq :)
(: Description: 

   To avoid unexpected errors caused by expression rewrite, tests that
   are designed to prevent dynamic errors should be expressed using
   conditional or typeswitch expressions.
:)

(: insert-start :)
(: insert-end :)

count(
 let $N :=  <n x="this ain't no date"/>
 return $N[if (@x castable as xs:date)
   then xs:date(@x) gt xs:date("2000-01-01")
   else false()]
)
