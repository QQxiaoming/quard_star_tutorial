(:*******************************************************:)
(: Test: VarDecl045.xq          :)
(: Written By: Ravindranath Chennoju                     :)
(: Date: Wed Jun 29 14:28:57 2005                        :)
(: Purpose - Variable with no type definition            :)
(:*******************************************************:)

declare variable $x := xs:date("1970-01-01Z") ; 
$x
