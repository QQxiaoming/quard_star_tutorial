(:*******************************************************:)
(:Test: fn-seconds-from-duration-18                      :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "seconds-from-duration" function:)
(:as part of a "numeric-equal" expression (le operator)  :) 
(:*******************************************************:)

fn:seconds-from-duration(xs:dayTimeDuration("P20DT03H09M20S")) le fn:seconds-from-duration(xs:dayTimeDuration("P21DT15H21M31S"))