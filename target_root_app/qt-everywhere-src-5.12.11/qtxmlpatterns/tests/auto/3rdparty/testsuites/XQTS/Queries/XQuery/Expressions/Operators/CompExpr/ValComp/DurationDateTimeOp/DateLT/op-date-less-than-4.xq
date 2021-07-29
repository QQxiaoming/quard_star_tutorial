(:*******************************************************:)
(:Test: op-date-less-than-4                              :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "date-less-than" function that  :)
(:return true and used together with fn:not (le operator):)
(:*******************************************************:)
 
fn:not(xs:date("2005-04-02Z") le xs:date("2005-04-02Z"))