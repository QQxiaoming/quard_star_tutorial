(:*******************************************************:)
(:Test: adjust-dateTime-to-timezone-16                   :)
(:Written By: Carmelo Montanez                           :)
(:Date: August 10, 2005                                  :)
(:Test Description: Evaluates string value The "adjust-dateTime-to-timezone" :)
(:function as part of a boolean (and) expression and the fn:true function. :)
(:*******************************************************:)

fn:string(fn:adjust-dateTime-to-timezone(xs:dateTime("2002-03-07T10:00:00-04:00"),())) and fn:true()