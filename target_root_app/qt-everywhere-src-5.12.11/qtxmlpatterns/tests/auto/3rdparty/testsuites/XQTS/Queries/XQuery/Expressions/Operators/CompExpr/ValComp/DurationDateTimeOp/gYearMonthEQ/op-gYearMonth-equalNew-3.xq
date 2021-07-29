(:*******************************************************:)
(:Test: op-gYearMonth-equal-3                            :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "gYearMonth-equal" function that:)
(:return true and used together with fn:not (eq operator):)
(:*******************************************************:)
 
fn:not((xs:gYearMonth("1995-02Z") eq xs:gYearMonth("1995-02Z")))