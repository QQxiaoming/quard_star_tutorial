(:*******************************************************:)
(: Test: VarDecl058.xq          :)
(: Written By: Ravindranath Chennoju                     :)
(: Date: Wed Jun 29 14:28:57 2005                        :)
(: Purpose - Variable with no type definition            :)
(:*******************************************************:)

declare variable $x := ((1,2,2),(1,2,3),(123,""),(),("")) ; 
$x
