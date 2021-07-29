(:*******************************************************:)
(:Test: op-gYearMonth-equal-6                            :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "gYearMonth-equal" function that:)
(:return false and used together with fn:not(ne operator):)
(:*******************************************************:)
 
fn:not(xs:gYearMonth("2005-01Z") ne xs:gYearMonth("2005-01Z"))