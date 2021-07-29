(:*******************************************************:)
(:Test: op-gYearMonth-equal-12                           :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "gYearMonth-equal" function used:)
(:together with "fn:true"/or expression (ne operator).   :)
(:*******************************************************:)
 
(xs:gYearMonth("2000-06Z") ne xs:gYearMonth("2000-07Z")) or (fn:true())