(:*******************************************************:)
(:Test: hours-from-dateTime-3                            :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 6, 2005                                     :)
(:Purpose: Evaluates The "hours-from-dateTime" function  :)
(:As per example 3 of the F&O  specs                     :)
(:*******************************************************:)

fn:hours-from-dateTime(fn:adjust-dateTime-to-timezone(xs:dateTime("1999-12-31T21:20:00-05:00"), xs:dayTimeDuration("PT0H")))