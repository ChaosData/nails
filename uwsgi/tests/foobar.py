def application(env, start_response):
    print env
    print dir(env)
    print env['wsgi.input']
    print dir(env['wsgi.input'])
    print env['wsgi.input'].read()
    start_response('200 OK', [('Content-Type','text/html')])
    #print env['wsgi.file_wrapper']('./foobar.py')
    return [b"Hello World"]

