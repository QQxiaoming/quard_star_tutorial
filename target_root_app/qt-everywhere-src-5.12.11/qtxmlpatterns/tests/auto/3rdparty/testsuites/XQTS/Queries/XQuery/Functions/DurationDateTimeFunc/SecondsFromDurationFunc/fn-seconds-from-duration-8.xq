(:*******************************************************:)
(:Test: fn-seconds-from-duration-8                       :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "seconds-from-duration" function:)
(:as part of a "+" expression.                           :) 
(:*******************************************************:)

fn:seconds-from-duration(xs:dayTimeDuration("P21DT10H10M09S")) + fn:seconds-from-duration(xs:dayTimeDuration("P22DT11H30M21S"))