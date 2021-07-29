(:*******************************************************:)
(:Test: op-date-greater-than-3                           :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "date-greater-than" function that:)
(:return true and used together with fn:not (gt operator):)
(:*******************************************************:)
 
fn:not((xs:date("2005-12-26Z") gt xs:date("2005-12-25Z")))