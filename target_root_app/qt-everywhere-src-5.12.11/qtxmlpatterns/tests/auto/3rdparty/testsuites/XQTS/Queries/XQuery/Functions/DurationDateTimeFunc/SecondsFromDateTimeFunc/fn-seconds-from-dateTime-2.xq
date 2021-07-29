(:*******************************************************:)
(:Test: seconds-from-dateTime-2                          :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 6, 2005                                     :)
(:Purpose: Evaluates The "seconds-from-dateTime" function:)
(:as an argument to an "avg" function.                   :)
(:*******************************************************:)

fn:avg((fn:seconds-from-dateTime(xs:dateTime("1999-05-31T13:30:10Z")), fn:seconds-from-dateTime(xs:dateTime("1999-05-31T13:30:15Z"))))