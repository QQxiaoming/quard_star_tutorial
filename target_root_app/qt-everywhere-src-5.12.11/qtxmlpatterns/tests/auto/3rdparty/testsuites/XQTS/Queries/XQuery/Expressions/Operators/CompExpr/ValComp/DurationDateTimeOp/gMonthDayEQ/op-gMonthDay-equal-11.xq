(:*******************************************************:)
(:Test: op-gMonthDay-equal-11                            :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 22, 2005                                    :)
(:Purpose: Evaluates The "gMonthDay-equal" function used :)
(:together with "fn:true"/or expression (eq operator).   :)
(:*******************************************************:)
 
(xs:gMonthDay("--03-04Z") eq xs:gMonthDay("--01-07Z")) or (fn:true())