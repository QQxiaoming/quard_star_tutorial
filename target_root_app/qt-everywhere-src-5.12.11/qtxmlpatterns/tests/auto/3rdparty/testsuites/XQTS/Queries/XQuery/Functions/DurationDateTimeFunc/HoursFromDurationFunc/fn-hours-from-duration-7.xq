(:*******************************************************:)
(:Test: fn-hours-from-duration-7                         :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "hours-from-duration" function  :)
(:used as arguments to an avg function.                  :) 
(:*******************************************************:)

fn:avg((fn:hours-from-duration(xs:dayTimeDuration("P23DT10H")),fn:hours-from-duration(xs:dayTimeDuration("P21DT08H")))) 