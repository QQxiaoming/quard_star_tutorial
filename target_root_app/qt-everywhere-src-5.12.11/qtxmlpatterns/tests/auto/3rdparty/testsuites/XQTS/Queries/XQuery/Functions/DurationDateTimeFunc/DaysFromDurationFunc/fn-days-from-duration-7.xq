(:*******************************************************:)
(:Test: fn-days-from-duration-7                          :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "days-from-duration" function   :)
(:used as arguments to an avg function.                  :) 
(:*******************************************************:)

fn:avg((fn:days-from-duration(xs:dayTimeDuration("P23DT10H")),fn:days-from-duration(xs:dayTimeDuration("P21DT10H")))) 