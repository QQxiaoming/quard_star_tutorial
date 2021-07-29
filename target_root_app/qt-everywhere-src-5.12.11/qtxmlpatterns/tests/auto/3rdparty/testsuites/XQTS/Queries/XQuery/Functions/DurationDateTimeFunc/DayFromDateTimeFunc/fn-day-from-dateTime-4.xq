(:*******************************************************:)
(:Test: fn-day-from-dateTime-4                           :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 8, 2005                                     :)
(:Purpose: Evaluates The "day-from-dateTime" function    :)
(:used as an argument to an "avg" function.              :)
(:*******************************************************:)

fn:avg((fn:day-from-dateTime(xs:dateTime("1996-12-10T12:00:00Z")),fn:day-from-dateTime(xs:dateTime("2000-12-20T12:00:00Z"))))