(:*******************************************************:)
(:Test: op-gYearMonth-equal-5                            :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "gYearMonth-equal" function that:)
(:return false and used together with fn:not (eq operator):)
(:*******************************************************:)
 
fn:not(xs:gYearMonth("2000-01Z") eq xs:gYearMonth("2001-04Z"))