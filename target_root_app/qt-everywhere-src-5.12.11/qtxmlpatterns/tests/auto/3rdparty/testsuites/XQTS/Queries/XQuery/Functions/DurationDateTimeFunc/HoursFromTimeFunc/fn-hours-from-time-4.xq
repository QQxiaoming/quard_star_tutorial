(:*******************************************************:)
(:Test: hours-from-time-4                                :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 6, 2005                                     :)
(:Purpose: Evaluates The "hours-from-time" function      :)
(:As per example 4 of the F&O  specs                     :)
(:*******************************************************:)

fn:hours-from-time(fn:adjust-time-to-timezone(xs:time("01:23:00+05:00"), xs:dayTimeDuration("PT0H"))) 