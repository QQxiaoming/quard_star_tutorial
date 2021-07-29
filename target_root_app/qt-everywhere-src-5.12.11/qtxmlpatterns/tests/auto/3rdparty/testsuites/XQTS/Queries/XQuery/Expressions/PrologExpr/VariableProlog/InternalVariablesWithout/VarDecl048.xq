(:*******************************************************:)
(: Test: VarDecl048.xq          :)
(: Written By: Ravindranath Chennoju                     :)
(: Date: Wed Jun 29 14:28:57 2005                        :)
(: Purpose - Variable with no type definition            :)
(:*******************************************************:)

declare variable $x := xs:time("08:03:35Z") + xs:dayTimeDuration("P0DT0H0M0S") ; 
$x
