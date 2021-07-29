(:*******************************************************:)
(: Test: VarDecl056.xq          :)
(: Written By: Ravindranath Chennoju                     :)
(: Date: Wed Jun 29 14:28:57 2005                        :)
(: Purpose - Variable with no type definition            :)
(:*******************************************************:)

declare variable $x := (xs:time("12:30:00") , xs:string(" ") , xs:decimal("2.000000000000002")) ; 
$x
