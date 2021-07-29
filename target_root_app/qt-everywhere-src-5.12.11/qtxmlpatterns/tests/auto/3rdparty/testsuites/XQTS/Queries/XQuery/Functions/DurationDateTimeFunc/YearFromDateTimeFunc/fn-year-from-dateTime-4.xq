(:*******************************************************:)
(:Test: year-from-dateTime-4                             :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 8, 2005                                     :)
(:Purpose: Evaluates The "year-from-dateTime" function   :)
(:used as an argument to an "avg" function.              :)
(:*******************************************************:)

fn:avg((fn:year-from-dateTime(xs:dateTime("1996-12-31T12:00:00Z")),fn:year-from-dateTime(xs:dateTime("2000-12-31T12:00:00Z"))))