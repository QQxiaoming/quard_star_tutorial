(:*******************************************************:)
(:Test: adjust-dateTime-to-timezone-11                   :)
(:Written By: Carmelo Montanez                           :)
(:Date: August 10, 2005                                  :)
(:Test Description: Evaluates The "adjust-dateTime-to-timezone" :)
(:function as an argument to a string function.           :)
(:*******************************************************:)

fn:string(fn:adjust-dateTime-to-timezone(xs:dateTime("2002-03-07T10:00:00-04:00"),()))