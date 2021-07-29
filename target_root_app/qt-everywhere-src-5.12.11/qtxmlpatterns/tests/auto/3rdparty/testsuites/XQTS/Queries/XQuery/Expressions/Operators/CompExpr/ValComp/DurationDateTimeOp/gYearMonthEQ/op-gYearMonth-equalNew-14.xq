(:*******************************************************:)
(:Test: op-gYearMonth-equal-14                           :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "gYearMonth-equal" function used:)
(:together with "fn:false"/or expression (ne operator).  :)
(:*******************************************************:)
 
(xs:gYearMonth("1980-03Z") ne xs:gYearMonth("1980-03Z")) or (fn:false())