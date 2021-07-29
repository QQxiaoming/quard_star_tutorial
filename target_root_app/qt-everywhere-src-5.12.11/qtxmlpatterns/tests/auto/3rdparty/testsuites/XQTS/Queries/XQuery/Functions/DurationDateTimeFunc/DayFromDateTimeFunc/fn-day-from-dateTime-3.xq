(:*******************************************************:)
(:Test: fn-day-from-dateTime-3                           :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 8, 2005                                     :)
(:Purpose: Evaluates The "day-from-dateTime" function    :)
(:As per example 3 (for this function) of the F&O  specs :)
(:*******************************************************:)

fn:day-from-dateTime(fn:adjust-dateTime-to-timezone(xs:dateTime("1999-12-31T19:20:00-05:00"), xs:dayTimeDuration("PT0H")))