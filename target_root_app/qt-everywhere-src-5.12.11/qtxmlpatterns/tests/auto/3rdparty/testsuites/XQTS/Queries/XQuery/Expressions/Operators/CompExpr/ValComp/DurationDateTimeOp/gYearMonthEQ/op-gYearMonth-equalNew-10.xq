(:*******************************************************:)
(:Test: op-gYearMonth-equal-10                           :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "gYearMonth-equal" function used:)
(:together with "or" expression (ne operator).           :)
(:*******************************************************:)
 
(xs:gYearMonth("1976-01Z") ne xs:gYearMonth("1976-02Z")) or (xs:gYearMonth("1980-03Z") ne xs:gYearMonth("1980-04Z"))