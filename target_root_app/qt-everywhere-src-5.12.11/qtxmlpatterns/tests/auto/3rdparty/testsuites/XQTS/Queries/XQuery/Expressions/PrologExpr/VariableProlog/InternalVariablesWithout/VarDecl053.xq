(:*******************************************************:)
(: Test: VarDecl053.xq          :)
(: Written By: Ravindranath Chennoju                     :)
(: Date: Wed Jun 29 14:28:57 2005                        :)
(: Purpose - Variable with no type definition            :)
(:*******************************************************:)

declare variable $x := (xs:float("INF") , xs:double("NaN")) ; 
$x
