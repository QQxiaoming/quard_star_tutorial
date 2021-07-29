(:*******************************************************:)
(:Test: fn-days-from-duration-4                          :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "days-from-duration" function   :)
(:involving a "numeric-less-than" operation (le operator):)
(:*******************************************************:)

fn:days-from-duration(xs:dayTimeDuration("P21DT10H")) le fn:days-from-duration(xs:dayTimeDuration("P22DT10H"))