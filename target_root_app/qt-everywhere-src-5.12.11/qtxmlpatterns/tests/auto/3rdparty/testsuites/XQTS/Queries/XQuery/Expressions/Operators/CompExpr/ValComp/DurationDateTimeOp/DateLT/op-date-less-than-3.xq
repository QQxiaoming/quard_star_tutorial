(:*******************************************************:)
(:Test: op-date-less-than-3                              :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "date-less-than" function that  :)
(:return true and used together with fn:not (lt operator):)
(:*******************************************************:)
 
fn:not((xs:date("2005-12-25Z") lt xs:date("2005-12-26Z")))