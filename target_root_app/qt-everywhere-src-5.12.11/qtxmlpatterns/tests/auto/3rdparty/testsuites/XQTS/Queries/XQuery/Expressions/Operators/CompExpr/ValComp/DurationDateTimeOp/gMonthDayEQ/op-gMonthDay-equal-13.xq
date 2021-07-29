(:*******************************************************:)
(:Test: op-gMonthDay-equal-13                            :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 22, 2005                                    :)
(:Purpose: Evaluates The "gMonthDay-equal" function used :)
(:together with "fn:false"/or expression (eq operator).  :)
(:*******************************************************:)
 
(xs:gMonthDay("--05-05Z") eq xs:gMonthDay("--05-05Z")) or (fn:false())