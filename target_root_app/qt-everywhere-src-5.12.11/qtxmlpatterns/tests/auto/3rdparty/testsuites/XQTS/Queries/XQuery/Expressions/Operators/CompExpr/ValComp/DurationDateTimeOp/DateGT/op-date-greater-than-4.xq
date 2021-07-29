(:*******************************************************:)
(:Test: op-date-less-than-4                              :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "date-less-than" function that  :)
(:return true and used together with fn:not (ge operator):)
(:*******************************************************:)
 
fn:not(xs:date("2005-04-02Z") ge xs:date("2005-04-02Z"))