(:*******************************************************:)
(:Test: op-gYearMonth-equal-11                           :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "gYearMonth-equal" function used:)
(:together with "fn:true"/or expression (eq operator).   :)
(:*******************************************************:)
 
(xs:gYearMonth("1980-05Z") eq xs:gYearMonth("1980-05Z")) or (fn:true())