(:*******************************************************:)
(:Test: op-gYearMonth-equal-13                           :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "gYearMonth-equal" function used:)
(:together with "fn:false"/or expression (eq operator).  :)
(:*******************************************************:)
 
(xs:gYearMonth("1980-09Z") eq xs:gYearMonth("1980-09Z")) or (fn:false())