(:*******************************************************:)
(:Test: op-gYearMonth-equal-4                            :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "gYearMonth-equal" function that:)
(:return true and used together with fn:not (le operator):)
(:*******************************************************:)
 
fn:not(xs:gYearMonth("2005-02Z") ne xs:gYearMonth("2006-03Z"))