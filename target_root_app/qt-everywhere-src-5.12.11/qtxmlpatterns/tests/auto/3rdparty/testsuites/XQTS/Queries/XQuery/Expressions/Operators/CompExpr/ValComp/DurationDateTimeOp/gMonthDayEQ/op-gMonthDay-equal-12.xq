(:*******************************************************:)
(:Test: op-gMonthDay-equal-12                            :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 22, 2005                                    :)
(:Purpose: Evaluates The "gMonthDay-equal" function used :)
(:together with "fn:true"/or expression (ne operator).   :)
(:*******************************************************:)
 
(xs:gMonthDay("--08-04Z") ne xs:gMonthDay("--07-12Z")) or (fn:true())