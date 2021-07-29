(:*******************************************************:)
(: Test: VarDecl049.xq          :)
(: Written By: Ravindranath Chennoju                     :)
(: Date: Wed Jun 29 14:28:57 2005                        :)
(: Purpose - Variable with no type definition            :)
(:*******************************************************:)

declare variable $x := xs:dateTime("2030-12-31T23:59:59Z") ; 
$x
