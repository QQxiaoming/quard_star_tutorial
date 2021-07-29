(:*******************************************************:)
(:Test: op-date-less-than-6                              :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "date-less-than" function that  :)
(:return false and used together with fn:not(le operator):)
(:*******************************************************:)
 
fn:not(xs:date("2005-10-25Z") le xs:date("2005-10-23Z"))