(:*******************************************************:)
(:Test: op-gMonthDay-equal-14                            :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 22, 2005                                    :)
(:Purpose: Evaluates The "gMonthDay-equal" function used :)
(:together with "fn:false"/or expression (ne operator).  :)
(:*******************************************************:)
 
(xs:gMonthDay("--09-09Z") ne xs:gMonthDay("--09-10Z")) or (fn:false())