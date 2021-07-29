(:*******************************************************:)
(:Test: seconds-from-time-4                              :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 6, 2005                                     :)
(:Purpose: Evaluates The "seconds-from-time" function    :)
(:used as an argument to an avg function.                :)
(:*******************************************************:)

fn:avg((fn:seconds-from-time(xs:time("01:10:20Z")), fn:seconds-from-time(xs:time("01:20:30Z"))))