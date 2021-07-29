(:*******************************************************:)
(:Test: op-date-greater-than-6                           :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "date-greater-than" function that:)
(:return false and used together with fn:not(ge operator):)
(:*******************************************************:)
 
fn:not(xs:date("2005-10-23Z") ge xs:date("2005-10-25Z"))