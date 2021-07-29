(:*******************************************************:)
(:Test: fn-month-from-dateTime-4                         :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 8, 2005                                     :)
(:Purpose: Evaluates The "month-from-dateTime" function  :)
(:used as an argument to an "avg" function.              :)
(:*******************************************************:)

fn:avg((fn:month-from-dateTime(xs:dateTime("1996-12-31T12:00:00Z")),fn:month-from-dateTime(xs:dateTime("2000-10-31T12:00:00Z"))))