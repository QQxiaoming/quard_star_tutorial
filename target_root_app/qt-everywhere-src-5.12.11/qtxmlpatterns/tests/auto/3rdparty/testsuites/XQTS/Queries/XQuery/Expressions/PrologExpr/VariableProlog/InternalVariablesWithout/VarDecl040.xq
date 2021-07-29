(:*******************************************************:)
(: Test: VarDecl040.xq          :)
(: Written By: Ravindranath Chennoju                     :)
(: Date: Wed Jun 29 14:28:57 2005                        :)
(: Purpose - Variable with no type definition            :)
(:*******************************************************:)

declare variable $x := xs:decimal("-999999999999999999") ; 
$x
